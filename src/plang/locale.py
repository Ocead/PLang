import locale
from enum import Enum


class WordOrder(Enum):
    SVO = 1


wordOrders = {
    'en': WordOrder.SVO
}

defaultLanguage = 'en'
