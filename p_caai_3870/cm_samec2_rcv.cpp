/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:
Version:    1.0
Date:     2019-8-21
Description: 物料价格接收信息
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
#include "epex.h" 
BM2F_ENTERACE_TELE(cm_samec2_rcv)

int f_cm_samec2_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CModel tcaac12("TCAAC12");
	int doFlag = 0;
	CString sqlstr = " ";
	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaac12.Reset();
			//tcaac12.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			if (bcls_rec->Tables[0].Rows[i]["BWKEY"].ToString() == "6767")
			{
				tcaac12["PRICE_TERMS"] = bcls_rec->Tables[0].Rows[i]["VPRSV"].ToString().Trim();
				if (tcaac12["PRICE_TERMS"].ToString() = 'S') //标准价，计划价
				{
					tcaac12["PRICE_TERMS"] = "CJ";
				}
				else if (tcaac12["PRICE_TERMS"].ToString() = 'V') //加权平均价消耗价格
				{
					tcaac12["PRICE_TERMS"] = "GJ";
				}
				Log::Trace("", __FUNCTION__, "PRICE_TERMS=[{0}]  ", tcaac12["PRICE_TERMS"].ToString().Trim());
				tcaac12["VALID_TIME_START"] = bcls_rec->Tables[0].Rows[i]["LAEPR"].ToString().Trim();
				tcaac12["MAT_CODE"] = bcls_rec->Tables[0].Rows[i]["MATNR"].ToString().Trim();
				tcaac12["MAT_NAME"] = bcls_rec->Tables[0].Rows[i]["MAKTX"].ToString().Trim();
				tcaac12["UNIT"] = bcls_rec->Tables[0].Rows[i]["MEINS"].ToString().Trim();
				tcaac12["PRICE_UNIT"] = bcls_rec->Tables[0].Rows[i]["STPRS"].ToDecimal();
				tcaac12["REC_REVISOR"] = bcls_rec->Tables[0].Rows[i]["USNAM"].ToString().Trim();

				tcaac12["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
				tcaac12["REC_CREATOR"] = s.userid;
				tcaac12.TrimOrBlank(); //去除多余的空格，如果是null的话字符型赋值空格或是数字小赋0

				if (tcaac12["PRICE_UNIT"].ToDecimal() > 0) //sap有发费用，单价是0,过滤
				{
					tcaac12.Delete("MAT_CODE,VALID_TIME_START,PRICE_TERMS");
					tcaac12.Insert();
				}
			}
		}
	}
	catch (CDbException& ex)
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}],请联系开发人员", arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strcpy(s.msg, ex.GetMsg());
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}


